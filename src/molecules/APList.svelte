<script lang="ts">
  import { onMount } from "svelte";
  import { List, ProgressLinear } from "smelte";
  import type { ListItemProps } from "smelte";

  let items: ListItemProps[] = [];
  let loading = false;

  function fetchApList() {
    loading = true;
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
