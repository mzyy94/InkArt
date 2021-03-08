<script lang="ts">
  import { onMount } from "svelte";
  import { get } from "../api/method";
  import UsageBar from "../molecules/UsageBar.svelte";
  import InfoLabel from "../atoms/InfoLabel.svelte";

  interface Info {
    system: {
      version: string;
      model: string;
    };
    storage: {
      used: number;
      total: number;
      photos: number;
    };
    network: {
      mac: string;
      ipv4: string;
    };
  }

  let info: Info | undefined;

  function initSettings() {
    get<Info>("/info.json").then((data) => {
      info = data;
    });
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
