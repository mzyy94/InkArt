<script lang="ts">
  import { onMount } from "svelte";
  import { Snackbar, Tabs, Tab } from "smelte";
  import dark from "smelte/src/dark";
  import api from "../api";
  import type { WiFiMode } from "../api";
  import WiFiInput from "../molecules/WiFiInput.svelte";
  import type { SubmitEvent } from "../molecules/WiFiInput.svelte";
  import APList from "../molecules/APList.svelte";

  const darkMode = dark();

  let ssid = "";
  let password = "";

  let mode: WiFiMode | "" = "";
  let processing = false;

  let snackbar = {
    show: false,
    text: "",
    color: "primary",
  };

  onMount(async () => {
    const wifi = await api.wifi();
    mode = wifi.mode;
    ssid = wifi.ssid;
  });

  function setupWiFi(e: SubmitEvent) {
    if (mode == "") {
      return;
    }
    let operation: string;
    if (mode == "sta") {
      operation = "Connection";
    } else {
      operation = "Create AP";
    }

    processing = true;
    const { ssid, password } = e.detail;

    api.wifi({ mode, ssid, password }).then((res) => {
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
