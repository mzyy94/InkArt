<script lang="ts">
  import { onDestroy, onMount } from "svelte";
  import { List } from "smelte";

  let items = [];

  const fetchApList = () =>
    fetch("/aplist.json")
      .then((res) => res.json())
      .then((list: { ssid: string; auth: string; rssi: number }[]) => {
        items = list
          .sort((a, b) => b.rssi - a.rssi)
          .map((item) => ({
            text: item.ssid,
            icon: item.auth == "open" ? "network_wifi" : "wifi_lock",
            subheading: `RSSI: ${item.rssi}dB`,
          }));
      });

  let interval: number;
  onMount(() => {
    fetchApList();
    interval = self.setInterval(fetchApList, 10_000);
  });
  onDestroy(() => self.clearInterval(interval));

  export let ssid: string;
</script>

<List dense {items} class="w-full" bind:value={ssid} />
