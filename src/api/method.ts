export function post(path: string, json: object) {
  return fetch(path, {
    method: "POST",
    headers: [["Content-Type", "application/json"]],
    body: JSON.stringify(json),
  });
}

export function get<T>(path: string): Promise<T> {
  return fetch(path, {
    headers: [["Accept", "application/json"]],
  }).then((res) => {
    if (!res.ok) {
      return Promise.reject(new Error(`request failed: GET ${path}`));
    }
    return res.json();
  });
}
