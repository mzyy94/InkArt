const databaseName = "photoplate";
const photoStoreName = "photos";
const hiddenStoreName = "hidden";

async function promisifyRequest<T>(
  request: IDBRequest<T>
): Promise<{ target: IDBRequest<T> }> {
  return new Promise((resolve, reject) => {
    // @ts-ignore
    request.onsuccess = resolve;
    request.onerror = reject;
  });
}

interface IDBObjectStorePro {
  add(value: any): Promise<{ target: IDBRequest<IDBValidKey> }>;
  get(query: IDBValidKey): Promise<{ target: IDBRequest<any | undefined> }>;
  getAll(query?: IDBValidKey): Promise<{ target: IDBRequest<any[]> }>;
  delete(key: IDBValidKey): Promise<{ target: IDBRequest<undefined> }>;
}

export async function openPhotoDatabase(mode: IDBTransactionMode) {
  const request = indexedDB.open(databaseName, 4);

  request.onupgradeneeded = function (event) {
    const db = (event.target as IDBOpenDBRequest).result;
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
      const close = () => {
        db.close();
      };
      const handler: ProxyHandler<IDBObjectStore> = {
        get: function (target, prop: keyof IDBObjectStore) {
          return (...args: any[]) =>
            promisifyRequest((target[prop] as Function)(...args));
        },
      };

      return {
        photo: (new Proxy(photo, handler) as unknown) as IDBObjectStorePro,
        hidden: (new Proxy(hidden, handler) as unknown) as IDBObjectStorePro,
        close,
      };
    })
    .catch((event) => {
      return Promise.reject(event.target.error);
    });
}
