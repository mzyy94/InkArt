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

  let snackbar = {
    show: false,
    text: "",
    color: "primary",
  };

  function connectWiFi(e: SubmitEvent) {
    const { ssid, password } = e.detail;
    fetch("/connect.json", {
      method: "POST",
      body: JSON.stringify({ ssid, password }),
    }).then((res) => {
      if (res.ok) {
        snackbar.text = "Connection succeeded";
        snackbar.color = "primary";
      } else {
        snackbar.text = "Connection failed";
        snackbar.color = "error";
      }
      snackbar.show = true;
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
      <WiFiInput {ssid} {password} button={"Connect"} on:submit={connectWiFi} />
      <APList bind:ssid />
    </Tab>
    <Tab id="ap" selected={mode}>
      <WiFiInput {ssid} {password} button={"Setup"} />
    </Tab>
  </div>
</Tabs>

<Snackbar color={snackbar.color} bind:value={snackbar.show}>
  <div>{snackbar.text}</div>
</Snackbar>
