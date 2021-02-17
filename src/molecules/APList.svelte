<script lang="ts">
  import { onDestroy, onMount } from "svelte";
  import { List, ProgressLinear } from "smelte";

  let items = [];
  let loading = !false;

  const fetchApList = () =>
    new Promise((resolve) => {
      loading = true;
      resolve(true);
    })
      .then(() => fetch("/aplist.json"))
      .then((res) => res.json())
      .then((list: { ssid: string; auth: string; rssi: number }[]) => {
        items = list
          .sort((a, b) => b.rssi - a.rssi)
          .map((item) => ({
            text: item.ssid,
            icon: item.auth == "open" ? "network_wifi" : "wifi_lock",
            subheading: `RSSI: ${item.rssi}dB`,
          }));
      })
      .then(() => {
        loading = false;
      });

  let interval: number;
  onMount(() => {
    fetchApList();
    interval = self.setInterval(fetchApList, 10_000);
  });
  onDestroy(() => self.clearInterval(interval));

  export let ssid: string;
</script>

<section class="mt-3">
  {#if loading}
    <ProgressLinear />
  {/if}
  <List dense {items} class="w-full" bind:value={ssid} />
</section>
