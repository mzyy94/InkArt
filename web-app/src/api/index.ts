import { get, patch, post } from "./method";

export interface Config {
  time: string;
  refresh: number;
}

export interface Display {
  inverted: boolean;
  orientation: Orientation;
  padding: {
    top: number;
    left: number;
    right: number;
    bottom: number;
  };
}

export type Orientation =
  | "portrait-right"
  | "upside-down"
  | "landscape"
  | "portrait-left";

export interface Entry {
  filename: string;
  date: string;
  hidden: boolean;
}

export interface PhotoEntry {
  data: Entry[];
}

export interface Info {
  system: {
    version: string;
    model: string;
  };
  display: {
    width: number;
    height: number;
  };
  storage: {
    used: number;
    total: number;
    photos: number;
  };
  network: {
    mac: string;
    ipv4: string;
  };
}

export interface OperationResult {
  status: "succeeded" | "failed";
  detail?: string;
}

type DeepPartial<T> = {
  [P in keyof T]?: DeepPartial<T[P]>;
};

function API<T extends { [key in string]: any }>(path: string) {
  return function <U>(
    ...value: [U] | []
  ): U extends T | DeepPartial<T> ? Promise<Response> : Promise<T> {
    if (value[0] === undefined) {
      return get<T>(path) as any;
    } else {
      const obj = value[0] as unknown as T;
      if (obj?.data?.length == 1) {
        return patch(path, obj) as any;
      }
      return post(path, obj) as any;
    }
  };
}

export default {
  config: API<Config>("/api/config.json"),
  display: API<Display>("/api/display.json"),
  photos: API<PhotoEntry>("/api/photos.json"),
  info: API<Info>("/api/info.json"),
} as const;
