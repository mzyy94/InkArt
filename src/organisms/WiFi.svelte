<script context="module" lang="ts">
  import type { SubmitEvent } from "../molecules/WiFiInput.svelte";
</script>

<script lang="ts">
  import { Snackbar, Tabs, Tab } from "smelte";
  import WiFiInput from "../molecules/WiFiInput.svelte";
  import APList from "../molecules/APList.svelte";

  let ssid = "";
  let password = "";

  let mode = "sta";
  let processing = false;

  let snackbar = {
    show: false,
    text: "",
    color: "primary",
  };

  function setupWiFi(e: SubmitEvent) {
    let endpoint: string;
    let operation: string;
    if (mode == "sta") {
      endpoint = "/connect.json";
      operation = "Connection";
    } else {
      endpoint = "/ap.json";
      operation = "Create AP";
    }

    processing = true;
    const { ssid, password } = e.detail;

    fetch(endpoint, {
      method: "POST",
      body: JSON.stringify({ ssid, password }),
    }).then((res) => {
      snackbar.text = `${operation} ${res.ok ? "succeeded" : "failed"}`;
      snackbar.color = res.ok ? "primary" : "error";
      snackbar.show = true;
      processing = false;
    });
  }
</script>

<Tabs
  bind:selected={mode}
  color="black"
  notSelectedColor="gray"
  indicator={false}
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
