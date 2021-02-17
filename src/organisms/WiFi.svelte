<script context="module" lang="ts">
  import type { SubmitEvent } from "../molecules/WiFiInput.svelte";
</script>

<script lang="ts">
  import { Tabs, Tab } from "smelte";
  import WiFiInput from "../molecules/WiFiInput.svelte";
  import APList from "../molecules/APList.svelte";

  let ssid = "";
  let password = "";

  let mode = "sta";

  function connectWiFi(e: SubmitEvent) {
    const { ssid, password } = e.detail;
    if (mode == "sta") {
      fetch("/connect.json", {
        method: "POST",
        body: JSON.stringify({ ssid, password }),
      }).then((res) => {
        console.log(res.ok);
      });
    } else {
      // TODO: Setup Wi-Fi
    }
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
