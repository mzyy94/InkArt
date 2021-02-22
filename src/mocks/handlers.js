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
const storeName = "photos";

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
    return new Promise((resolve, reject) => {
      const request = indexedDB.open(databaseName, 2);

      request.onerror = function(event) {
        reject(res(ctx.status(500), ctx.json({
          status: "failed",
          detail: /** @type {IDBOpenDBRequest} */(event.target).error
        })))
      };
      request.onupgradeneeded = function(event) {
        const db = /** @type {IDBOpenDBRequest} */(event.target).result;
        const objectStore = db.createObjectStore(storeName, { keyPath: "filename" });
        objectStore.createIndex("date", "date", { unique: false });
      };
      request.onsuccess = function(event) {
        const filename = `image-${Date.now()}.png`;
        const db = /** @type {IDBOpenDBRequest} */(event.target).result;
        const transaction = db.transaction([storeName], "readwrite");
        const store = transaction.objectStore(storeName);
        store.add({filename, date: new Date(), data: req.body});
        transaction.oncomplete = function() {
          db.close();
          resolve(res(ctx.delay(3000), ctx.status(200), ctx.json({ status: "succeeded", filename })))
        }
        transaction.onerror = function(event) {
          db.close();
          reject(res(ctx.status(500), ctx.json({
            status: "failed",
            detail: /** @type {IDBTransaction} */(event.target).error
          })))
        }
      }
    })
  }),
];
