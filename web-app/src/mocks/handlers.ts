import { rest } from "msw";
import type { ResponseComposition, RestContext } from "msw";
import { openPhotoDatabase } from "./db";
import type {
  PhotoEntry,
  Display,
  Orientation,
  Config,
  Info,
  OperationResult,
} from "../api";

function handle500ErrorResponse(res: ResponseComposition, ctx: RestContext) {
  return function (error: Error) {
    console.error(error);
    return res(
      ctx.status(500),
      ctx.json<OperationResult>({
        status: "failed",
        detail: error.toString(),
      })
    );
  };
}

export const handlers = [
  rest.post<string>("/api/upload", async (req, res, ctx) => {
    if (!req.body) {
      return res(
        ctx.status(400),
        ctx.json<OperationResult>({
          status: "failed",
          detail: "file not found",
        })
      );
    }
    const blob = await fetch(`data:image/bmp;base64,` + req.body).then(
      (response) => response.blob()
    );
    const file = new File([blob], `image-${Date.now()}.bmp`);
    return openPhotoDatabase("readwrite")
      .then(async ({ photo, close }) => photo.add(file).finally(close))
      .then(() =>
        res(
          ctx.delay(3000),
          ctx.status(200),
          ctx.json<OperationResult>({ status: "succeeded" })
        )
      )
      .catch(handle500ErrorResponse(res, ctx));
  }),
  rest.get("/api/photos.json", (_req, res, ctx) => {
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
              date: new Date(file.lastModified).toJSON(),
              hidden: hiddenList.has(file.name),
            }));
            return res(
              ctx.delay(2000),
              ctx.status(200),
              ctx.json<PhotoEntry>({ data: filelist })
            );
          })
      )
      .catch(handle500ErrorResponse(res, ctx));
  }),
  rest.get("/api/photos/:filename", (req, res, ctx) => {
    return openPhotoDatabase("readonly")
      .then(({ photo, close }) => photo.get(req.params.filename).finally(close))
      .then(async ({ target: { result: data } }) => {
        if (!data) {
          return res(ctx.status(404), ctx.body(""));
        }
        const file = data as File;
        const buffer = await new Promise<ArrayBuffer>((resolve, reject) => {
          const fr = new FileReader();
          fr.onload = () => resolve(fr.result as ArrayBuffer);
          fr.onerror = () => reject(fr.error);
          fr.readAsArrayBuffer(file);
        });
        return res(
          ctx.status(200),
          ctx.set("Content-Length", buffer.byteLength.toString()),
          ctx.set("Content-Type", "image/bmp"),
          ctx.body(buffer)
        );
      })
      .catch(handle500ErrorResponse(res, ctx));
  }),
  rest.delete("/api/photos/:filename", (req, res, ctx) => {
    return openPhotoDatabase("readwrite").then(({ photo, close }) =>
      photo
        .get(req.params.filename)
        .then(({ target: { result: data } }) => {
          if (!data) {
            return res(
              ctx.status(404),
              ctx.json<OperationResult>({ status: "failed" })
            );
          }
          return photo
            .delete(req.params.filename)
            .finally(close)
            .then(() =>
              res(
                ctx.status(200),
                ctx.json<OperationResult>({ status: "succeeded" })
              )
            );
        })
        .catch(handle500ErrorResponse(res, ctx))
    );
  }),
  rest.patch<PhotoEntry>("/api/photos.json", (req, res, ctx) => {
    const [{ hidden: hide, filename }] = req.body.data;
    return openPhotoDatabase("readwrite")
      .then(({ photo, hidden, close }) =>
        photo
          .get(filename)
          .then(({ target: { result: data } }) => {
            if (!data) {
              return res(
                ctx.status(404),
                ctx.json<OperationResult>({ status: "failed" })
              );
            }
            const request = hide
              ? hidden.add({ name: filename })
              : hidden.delete(filename);

            return request.finally(close) as Promise<any>;
          })
          .then(() =>
            res(
              ctx.status(200),
              ctx.json<OperationResult>({ status: "succeeded" })
            )
          )
      )
      .catch(handle500ErrorResponse(res, ctx));
  }),
  rest.get("/api/display.json", (_req, res, ctx) => {
    const inverted = sessionStorage.getItem("inverted") == "true";
    const orientation =
      (sessionStorage.getItem("orientation") as Orientation | null) ??
      "landscape";
    const top = parseInt(sessionStorage.getItem("paddingTop") ?? "0");
    const left = parseInt(sessionStorage.getItem("paddingLeft") ?? "0");
    const right = parseInt(sessionStorage.getItem("paddingRight") ?? "0");
    const bottom = parseInt(sessionStorage.getItem("paddingBottom") ?? "0");
    return res(
      ctx.status(200),
      ctx.json<Display>({
        inverted,
        orientation,
        padding: { top, left, right, bottom },
      })
    );
  }),
  rest.post<Display>("/api/display.json", (req, res, ctx) => {
    const {
      inverted,
      orientation,
      padding: { top, left, right, bottom },
    } = req.body;
    sessionStorage.setItem("inverted", inverted.toString());
    sessionStorage.setItem("orientation", orientation);
    sessionStorage.setItem("paddingTop", top.toString(10));
    sessionStorage.setItem("paddingLeft", left.toString(10));
    sessionStorage.setItem("paddingRight", right.toString(10));
    sessionStorage.setItem("paddingBottom", bottom.toString(10));
    return res(
      ctx.status(200),
      ctx.json<OperationResult>({ status: "succeeded" })
    );
  }),
  rest.get("/api/config.json", (_req, res, ctx) => {
    const offset = parseInt(sessionStorage.getItem("offset") || "0", 10);
    const time = new Date(Date.now() + offset).toJSON();
    return res(ctx.status(200), ctx.json<Config>({ time }));
  }),
  rest.post<Config>("/api/config.json", (req, res, ctx) => {
    const time = new Date(req.body.time);
    const diff = time.getTime() - Date.now();
    sessionStorage.setItem("offset", diff.toString(10));
    return res(
      ctx.status(200),
      ctx.json<OperationResult>({ status: "succeeded" })
    );
  }),
  rest.get("/api/info.json", (_req, res, ctx) => {
    const version = "1.0";
    const model = "Inkplate 6";
    const mac = "00:11:22:33:44:55";
    const ipv4 = "192.168.123.234";
    const photos = 241;
    const used = 1000 * 1000 * 413;
    const total = 1000 * 1000 * 1000 * 4;
    return res(
      ctx.status(200),
      ctx.json<Info>({
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
