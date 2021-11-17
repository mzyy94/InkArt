<script lang="ts">
  import { onMount } from "svelte";
  import api from "../api";
  import type { Info } from "../api";
  import UsageBar from "../molecules/UsageBar.svelte";
  import InfoLabel from "../atoms/InfoLabel.svelte";

  let info: Info | undefined;

  async function initSettings() {
    info = await api.info();
  }

  onMount(initSettings);
</script>

<InfoLabel icon="memory" title="System Version">
  {info?.system.version}
</InfoLabel>
<InfoLabel icon="devices" title="Model">
  {info?.system.model}
</InfoLabel>
<InfoLabel icon="settings_ethernet" title="Mac Address">
  {info?.network.mac}
</InfoLabel>
<InfoLabel icon="wifi" title="IPv4 Address">
  {info?.network.ipv4}
</InfoLabel>
<InfoLabel icon="photo" title="Number of Photos">
  {new Intl.NumberFormat("en-US").format(info?.storage.photos ?? 0)}
</InfoLabel>
<UsageBar used={info?.storage.used ?? 0} total={info?.storage.total ?? 0} />
