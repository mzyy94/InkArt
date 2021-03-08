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
        darkmode: sessionStorage.getItem("inverted") == "true",
      })
    );
  }),
  rest.get("/aplist.json", (_req, res, ctx) => {
    const data = Array.from({ length: 6 }, (_, i) => {
      const ssid = names[i];
      const auth = enc[i];
      const rssi = -50 - ((Math.random() * 50) | 0);
      return { ssid, auth, rssi };
    });

    return res(ctx.delay(2000), ctx.status(200), ctx.json({ data }));
  }),
  rest.post<{ [key in string]: string }>("/wifi.json", (req, res, ctx) => {
    const { mode, ssid, password } = req.body;

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
  rest.patch<{
    data: [
      {
        filename: string;
        hidden: boolean;
      }
    ];
  }>("/photos.json", (req, res, ctx) => {
    const [{ hidden: hide, filename }] = req.body.data;
    return openPhotoDatabase("readwrite")
      .then(({ photo, hidden, close }) =>
        photo
          .get(filename)
          .then(({ target: { result: data } }) => {
            if (!data) {
              return res(ctx.status(404), ctx.json({ status: "failed" }));
            }
            const request = hide
              ? hidden.add({ name: filename })
              : hidden.delete(filename);

            return request.finally(close) as Promise<any>;
          })
          .then(() => res(ctx.status(200), ctx.json({ status: "succeeded" })))
      )
      .catch(handle500ErrorResponse(res, ctx));
  }),
  rest.get("/display.json", (_req, res, ctx) => {
    const inverted = sessionStorage.getItem("inverted") == "true";
    const orientation = sessionStorage.getItem("orientation") ?? "portrait";
    const interval = parseInt(sessionStorage.getItem("interval") ?? "30");
    const margin = parseInt(sessionStorage.getItem("margin") ?? "0");
    return res(
      ctx.status(200),
      ctx.json({ inverted, orientation, interval, margin })
    );
  }),
  rest.post<{
    inverted: boolean;
    orientation: string;
    interval: number;
    margin: number;
  }>("/display.json", (req, res, ctx) => {
    const { inverted, orientation, interval, margin } = req.body;
    sessionStorage.setItem("inverted", inverted.toString());
    sessionStorage.setItem("orientation", orientation);
    sessionStorage.setItem("interval", interval.toString(10));
    sessionStorage.setItem("margin", margin.toString(10));
    return res(ctx.status(200), ctx.json({ status: "succeeded" }));
  }),
  rest.get("/config.json", (_req, res, ctx) => {
    const offset = parseInt(sessionStorage.getItem("offset") || "0", 10);
    const time = new Date(Date.now() + offset);
    return res(ctx.status(200), ctx.json({ time }));
  }),
  rest.post<{ time: string }>("/config.json", (req, res, ctx) => {
    const time = new Date(req.body.time);
    const diff = time.getTime() - Date.now();
    sessionStorage.setItem("offset", diff.toString(10));
    return res(ctx.status(200), ctx.json({ status: "succeeded" }));
  }),
  rest.get("/info.json", (_req, res, ctx) => {
    const version = "1.0";
    const model = "Inkplate 6";
    const mac = "00:11:22:33:44:55";
    const ipv4 = "192.168.123.234";
    const photos = 241;
    const used = 1000 * 1000 * 413;
    const total = 1000 * 1000 * 1000 * 4;
    return res(
      ctx.status(200),
      ctx.json({
        system: {
          version,
          model,
        },
        storage: {
          used,
          total,
          photos,
        },
        network: {
          mac,
          ipv4,
        },
      })
    );
  }),
];
