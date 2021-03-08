export function post(path: string, json: object) {
  return fetch(path, {
    method: "POST",
    headers: [["Content-Type", "application/json"]],
    body: JSON.stringify(json),
  });
}
