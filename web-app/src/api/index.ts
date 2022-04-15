import { get, patch, post } from "./method";

export interface TimeConfig {
  time: number;
  refresh: number;
  shuffle: boolean;
}

export interface Display {
  invert: boolean;
  dithering: boolean;
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
  storage?: {
    used: number;
    total: number;
  };
  photos: number;
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
  config: API<TimeConfig>("/api/v1/system/time"),
  display: API<Display>("/api/v1/system/display"),
  preview: API<Display>("/api/v1/system/display/preview"),
  photos: API<PhotoEntry>("/api/v1/photos"),
  info: API<Info>("/api/v1/system/info"),
} as const;
