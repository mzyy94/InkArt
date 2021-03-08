<script lang="ts">
  import { onMount } from "svelte";
  import { Snackbar, Tabs, Tab } from "smelte";
  import dark from "smelte/src/dark";
  import { get, post } from "../api/method";
  import WiFiInput from "../molecules/WiFiInput.svelte";
  import type { SubmitEvent } from "../molecules/WiFiInput.svelte";
  import APList from "../molecules/APList.svelte";

  const darkMode = dark();

  let ssid = "";
  let password = "";

  type Mode = "sta" | "ap";
  let mode: Mode | "" = "";
  let processing = false;

  let snackbar = {
    show: false,
    text: "",
    color: "primary",
  };

  onMount(() => {
    get<{ mode: Mode; ssid: string }>("/wifi.json").then((wifi) => {
      mode = wifi.mode;
      ssid = wifi.ssid;
    });
  });

  function setupWiFi(e: SubmitEvent) {
    let operation: string;
    if (mode == "sta") {
      operation = "Connection";
    } else {
      operation = "Create AP";
    }

    processing = true;
    const { ssid, password } = e.detail;

    post("/wifi.json", { mode, ssid, password }).then((res) => {
      snackbar.text = `${operation} ${res.ok ? "succeeded" : "failed"}`;
      snackbar.color = res.ok ? "primary" : "error";
      snackbar.show = true;
      processing = false;
    });
  }
</script>

<Tabs
  bind:selected={mode}
  color={$darkMode ? "white" : "black"}
  notSelectedColor="gray"
  indicator={false}
  loading={mode == ""}
  items={[
    { id: "sta", text: "STA", icon: "tap_and_play" },
    { id: "ap", text: "AP", icon: "router" },
  ]}
>
  <div slot="content">
    <Tab id="sta" selected={mode}>
      <WiFiInput
        {ssid}
        {password}
        pending={processing}
        button={"Connect"}
        on:submit={setupWiFi}
      />
      <APList bind:ssid />
    </Tab>
    <Tab id="ap" selected={mode}>
      <WiFiInput
        {ssid}
        {password}
        pending={processing}
        button={"Create"}
        on:submit={setupWiFi}
      />
    </Tab>
  </div>
</Tabs>

<Snackbar color={snackbar.color} bind:value={snackbar.show}>
  <div>{snackbar.text}</div>
</Snackbar>
