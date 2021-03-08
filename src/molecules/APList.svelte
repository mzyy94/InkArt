<script lang="ts">
  import { onMount } from "svelte";
  import { List, ProgressLinear } from "smelte";
  import type { ListItemProps } from "smelte";
  import { get } from "../api/method";

  let items: ListItemProps[] = [];
  let loading = false;

  function fetchApList() {
    loading = true;
    get<{ ssid: string; auth: string; rssi: number }[]>("/aplist.json")
      .then((list) => {
        items = list
          .sort((a, b) => b.rssi - a.rssi)
          .map((item) => ({
            text: item.ssid,
            icon: item.auth == "open" ? "network_wifi" : "wifi_lock",
            subheading: `RSSI: ${item.rssi}dB`,
          }));
      })
      .finally(() => {
        loading = false;
      });
  }

  onMount(() => {
    fetchApList();
    const interval = self.setInterval(fetchApList, 10_000);
    return () => {
      self.clearInterval(interval);
    };
  });

  export let ssid: string;
</script>

<section class="mt-3">
  {#if loading}
    <ProgressLinear />
  {/if}
  <List dense {items} class="w-full" bind:value={ssid} />
</section>
