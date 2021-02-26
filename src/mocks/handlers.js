import { rest } from "msw";

const enc = [
  "open",
  "WEP",
  "WPA_PSK",
  "WPA2_PSK",
  "WPA_WPA2_PSK",
  "WPA2_ENTERPRISE",
];

const names = [
  "Panther",
  "Tiger",
  "Leopard",
  "Snow Leopard",
  "Lion",
  "Mountain Lion",
];

const databaseName = "photoplate";
const photoStoreName = "photos";
const hiddenStoreName = "hidden";

/**
 * @template T
 * @param {IDBRequest<T>} request
 * @returns {Promise<{target: IDBRequest<T>}>}
 */
async function promisifyRequest(request) {
  return new Promise((resolve, reject) => {
    request.onsuccess = resolve;
    request.onerror = reject;
  });
}

/**
 * @param {IDBTransactionMode} mode
 * @returns {!Promise<{photo: IDBObjectStore, hidden: IDBObjectStore}>}
 */
async function openPhotoDatabase(mode) {
  const request = indexedDB.open(databaseName, 4);

  request.onupgradeneeded = function (event) {
    const db = /** @type {IDBOpenDBRequest} */ (event.target).result;
    const objectStore = db.createObjectStore(photoStoreName, {
      keyPath: "name",
    });
    objectStore.createIndex("date", "lastModified", { unique: false });
    db.createObjectStore(hiddenStoreName, { keyPath: "name" });
  };

  return promisifyRequest(request)
    .then((event) => {
      const db = event.target.result;
      const transaction = db.transaction(
        [photoStoreName, hiddenStoreName],
        mode
      );
      const photo = transaction.objectStore(photoStoreName);
      const hidden = transaction.objectStore(hiddenStoreName);
      return { photo, hidden };
    })
    .catch((event) => {
      return Promise.reject(event.target.error);
    });
}

/**
 * @param {File} file
 */
async function getArrayBufferFrom7bitFile(file) {
  const buffer = await new Promise((resolve, reject) => {
    const fr = new FileReader();
    fr.onload = () => resolve(fr.result);
    fr.onerror = () => reject(fr.error);
    fr.readAsArrayBuffer(file);
  });
  const markers = new Set();
  const data = new Uint8Array(buffer)
    .map((c, i, a) => {
      if (c < 194) {
        return c;
      }
      markers.add(i + 1);
      return c == 194 ? a[i + 1] : a[i + 1] + 64;
    })
    .filter((_, i) => !markers.has(i));
  return data.buffer;
}

export const handlers = [
  rest.get("/status.json", (_req, res, ctx) => {
    return res(
      ctx.status(200),
      ctx.json({
        status: "ok",
      })
    );
  }),
  rest.get("/aplist.json", (_req, res, ctx) => {
    const list = Array.from({ length: 6 }, (_, i) => {
      const ssid = names[i];
      const auth = enc[i];
      const rssi = -50 - ((Math.random() * 50) | 0);
      return { ssid, auth, rssi };
    });

    return res(ctx.delay(2000), ctx.status(200), ctx.json(list));
  }),
  rest.post("/connect.json", (req, res, ctx) => {
    const { ssid, password } = JSON.parse(req.body);

    if (ssid.length == password.length) {
      sessionStorage.setItem("wifi-mode", "sta");
      return res(
        ctx.delay(4000),
        ctx.status(200),
        ctx.json({ status: "connected" })
      );
    } else {
      return res(
        ctx.delay(4000),
        ctx.status(403),
        ctx.json({ status: "failed" })
      );
    }
  }),
  rest.post("/ap.json", (req, res, ctx) => {
    const { ssid, password } = JSON.parse(req.body);

    if (ssid.length == password.length) {
      sessionStorage.setItem("wifi-mode", "ap");
      return res(
        ctx.delay(4000),
        ctx.status(200),
        ctx.json({ status: "created" })
      );
    } else {
      return res(
        ctx.delay(4000),
        ctx.status(403),
        ctx.json({ status: "failed" })
      );
    }
  }),
  rest.get("/wifi.json", (_req, res, ctx) => {
    const mode = sessionStorage.getItem("wifi-mode") ?? "ap";
    return res(ctx.status(200), ctx.json({ mode }));
  }),
  rest.post("/photos.json", (req, res, ctx) => {
    return openPhotoDatabase("readwrite")
      .then(({ photo }) => {
        if (!req.body.file) {
          return res(
            ctx.status(400),
            ctx.json({ status: "failed", detail: "file not found" })
          );
        }
        const file = /** @type {File} */ (req.body.file);
        const request = photo.add(file);
        return promisifyRequest(request)
          .then(() =>
            res(
              ctx.delay(3000),
              ctx.status(200),
              ctx.json({ status: "succeeded", filename: file.name })
            )
          )
          .catch((event) => Promise.reject(event.target.error))
          .finally(() => {
            photo.transaction.db.close();
          });
      })
      .catch((error) =>
        res(
          ctx.status(500),
          ctx.json({
            status: "failed",
            detail: error,
          })
        )
      );
  }),
  rest.get("/photos.json", (_req, res, ctx) => {
    return openPhotoDatabase("readonly")
      .then(({ photo }) => {
        const request = photo.getAll();
        return promisifyRequest(request)
          .then(({ target: { result: data } }) => {
            const filelist = (data ?? []).map((file) => ({
              filename: file.name,
              date: new Date(file.lastModified),
            }));
            return res(
              ctx.delay(2000),
              ctx.status(200),
              ctx.json({ status: "ok", data: filelist })
            );
          })
          .catch((event) => Promise.reject(event.target.error))
          .finally(() => {
            photo.transaction.db.close();
          });
      })
      .catch((error) =>
        res(
          ctx.status(500),
          ctx.json({
            status: "failed",
            detail: error,
          })
        )
      );
  }),
  rest.get("/photos/:filename", (req, res, ctx) => {
    return openPhotoDatabase("readonly")
      .then(({ photo }) => {
        const request = photo.get(req.params.filename);
        return promisifyRequest(request)
          .then(async ({ target: { result: data } }) => {
            if (!data) {
              return res(ctx.status(404), ctx.body(""));
            }
            const file = /** @type {File} */ (data);
            const buffer = await getArrayBufferFrom7bitFile(file);
            return res(
              ctx.status(200),
              ctx.set("Content-Length", buffer.byteLength.toString()),
              ctx.set("Content-Type", "image/bmp"),
              ctx.body(buffer)
            );
          })
          .catch((event) => Promise.reject(event.target.error))
          .finally(() => {
            photo.transaction.db.close();
          });
      })
      .catch((error) =>
        res(
          ctx.status(500),
          ctx.json({
            status: "failed",
            detail: error,
          })
        )
      );
  }),
  rest.delete("/photos/:filename", (req, res, ctx) => {
    return openPhotoDatabase("readwrite")
      .then(({ photo }) => {
        const request = photo.get(req.params.filename);
        return promisifyRequest(request)
          .then(({ target: { result: data } }) => {
            if (!data) {
              return res(ctx.status(404), ctx.json({ status: "failed" }));
            }
            const request = photo.delete(req.params.filename);
            return promisifyRequest(request).then(() =>
              res(ctx.status(200), ctx.json({ status: "succeeded" }))
            );
          })
          .catch((event) => Promise.reject(event.target.error))
          .finally(() => {
            photo.transaction.db.close();
          });
      })
      .catch((error) =>
        res(
          ctx.status(500),
          ctx.json({
            status: "failed",
            detail: error,
          })
        )
      );
  }),
  rest.put("/photos/:filename", (req, res, ctx) => {
    return openPhotoDatabase("readwrite")
      .then(({ photo, hidden }) => {
        const request = photo.get(req.params.filename);
        const { hide } = JSON.parse(req.body);
        return promisifyRequest(request)
          .then(({ target: { result: data } }) => {
            if (!data) {
              return res(ctx.status(404), ctx.json({ status: "failed" }));
            }
            const request = hide
              ? hidden.add({ name: req.params.filename })
              : hidden.delete(req.params.filename);

            return promisifyRequest(request).then(() =>
              res(ctx.status(200), ctx.json({ status: "succeeded" }))
            );
          })
          .catch((event) => Promise.reject(event.target.error))
          .finally(() => {
            hidden.transaction.db.close();
          });
      })
      .catch((error) =>
        res(
          ctx.status(500),
          ctx.json({
            status: "failed",
            detail: error,
          })
        )
      );
  }),
];
