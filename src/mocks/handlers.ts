import { rest, ResponseComposition, RestContext } from "msw";
import { readFileAsArrayBuffer, fixBrokenFile } from "./file";
import { openPhotoDatabase } from "./db";

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

function handle500ErrorResponse(res: ResponseComposition, ctx: RestContext) {
  return function (error: Error) {
    console.error(error);
    return res(
      ctx.status(500),
      ctx.json({
        status: "failed",
        detail: error,
      })
    );
  };
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
  rest.post<string>("/wifi.json", (req, res, ctx) => {
    const { mode, ssid, password } = JSON.parse(req.body);

    if (ssid.length == password.length) {
      sessionStorage.setItem("wifi-mode", mode);
      sessionStorage.setItem("ssid", ssid);
      return res(
        ctx.delay(4000),
        ctx.status(200),
        ctx.json({ status: "succeeded" })
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
    const ssid = sessionStorage.getItem("ssid") ?? "TEST_AP_1234";
    return res(ctx.status(200), ctx.json({ mode, ssid }));
  }),
  rest.post<{ file: File }>("/photos.json", async (req, res, ctx) => {
    if (!req.body.file) {
      return res(
        ctx.status(400),
        ctx.json({ status: "failed", detail: "file not found" })
      );
    }
    const file = await fixBrokenFile(req.body.file);

    return openPhotoDatabase("readwrite")
      .then(async ({ photo, close }) => photo.add(file).finally(close))
      .then(() =>
        res(
          ctx.delay(3000),
          ctx.status(200),
          ctx.json({ status: "succeeded", filename: name })
        )
      )
      .catch(handle500ErrorResponse(res, ctx));
  }),
  rest.get("/photos.json", (_req, res, ctx) => {
    return openPhotoDatabase("readonly")
      .then(({ photo, hidden, close }) =>
        Promise.all([photo.getAll(), hidden.getAll()])
          .finally(close)
          .then(([photos, hiddens]) => {
            const hiddenList = new Set(
              hiddens.target.result.map((item) => item.name)
            );
            const filelist = (photos.target.result ?? []).map((file) => ({
              filename: file.name,
              date: new Date(file.lastModified),
              hidden: hiddenList.has(file.name),
            }));
            return res(
              ctx.delay(2000),
              ctx.status(200),
              ctx.json({ status: "ok", data: filelist })
            );
          })
      )
      .catch(handle500ErrorResponse(res, ctx));
  }),
  rest.get("/photos/:filename", (req, res, ctx) => {
    return openPhotoDatabase("readonly")
      .then(({ photo, close }) => photo.get(req.params.filename).finally(close))
      .then(async ({ target: { result: data } }) => {
        if (!data) {
          return res(ctx.status(404), ctx.body(""));
        }
        const file = data as File;
        const buffer = await readFileAsArrayBuffer(file);
        return res(
          ctx.status(200),
          ctx.set("Content-Length", buffer.byteLength.toString()),
          ctx.set("Content-Type", "image/bmp"),
          ctx.body(buffer)
        );
      })
      .catch(handle500ErrorResponse(res, ctx));
  }),
  rest.delete("/photos/:filename", (req, res, ctx) => {
    return openPhotoDatabase("readwrite").then(({ photo, close }) =>
      photo
        .get(req.params.filename)
        .then(({ target: { result: data } }) => {
          if (!data) {
            return res(ctx.status(404), ctx.json({ status: "failed" }));
          }
          return photo
            .delete(req.params.filename)
            .finally(close)
            .then(() =>
              res(ctx.status(200), ctx.json({ status: "succeeded" }))
            );
        })
        .catch(handle500ErrorResponse(res, ctx))
    );
  }),
  rest.put<string>("/photos/:filename", (req, res, ctx) => {
    const { hide } = JSON.parse(req.body);
    return openPhotoDatabase("readwrite")
      .then(({ photo, hidden, close }) =>
        photo
          .get(req.params.filename)
          .then(({ target: { result: data } }) => {
            if (!data) {
              return res(ctx.status(404), ctx.json({ status: "failed" }));
            }
            const request = hide
              ? hidden.add({ name: req.params.filename })
              : hidden.delete(req.params.filename);

            return request.finally(close) as Promise<any>;
          })
          .then(() => res(ctx.status(200), ctx.json({ status: "succeeded" })))
      )
      .catch(handle500ErrorResponse(res, ctx));
  }),
];
