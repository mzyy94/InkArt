import { get, patch, post } from "./method";

export interface AccessPoint {
  ssid: string;
  auth: string;
  rssi: number;
}

export interface AccessPointList {
  data: AccessPoint[];
}

export interface Config {
  time: string;
}

export interface Display {
  inverted: boolean;
  orientation: Orientation;
  interval: number;
  margin: number;
}

export type Orientation =
  | "portrait"
  | "landscape-right"
  | "landscape-left"
  | "upside-down";

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

export interface WiFi {
  mode: WiFiMode;
  ssid: string;
  password?: string;
}

export type WiFiMode = "sta" | "ap";

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
      const obj = (value[0] as unknown) as T;
      if (obj?.data?.length == 1) {
        return patch(path, obj) as any;
      }
      return post(path, obj) as any;
    }
  };
}

export default {
  apList: API<AccessPointList>("/api/aplist.json"),
  config: API<Config>("/api/config.json"),
  display: API<Display>("/api/display.json"),
  photos: API<PhotoEntry>("/api/photos.json"),
  info: API<Info>("/api/info.json"),
  wifi: API<WiFi>("/api/wifi.json"),
} as const;
