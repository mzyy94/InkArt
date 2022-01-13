<script lang="ts">
  import { onMount } from "svelte";
  import api from "../../api";
  import type { Info } from "../../api";
  import Container from "../templates/Container.svelte";
  import UsageBar from "../molecules/UsageBar.svelte";
  import InfoLabel from "../atoms/InfoLabel.svelte";

  let info: Info | undefined;

  async function initSettings() {
    info = await api.info();
  }

  onMount(initSettings);
</script>

<main>
  <Container>
    <span slot="title">Information</span>
    <InfoLabel icon="memory" title="System Version">
      {info?.system.version}
    </InfoLabel>
    <InfoLabel icon="devices" title="Model">
      {info?.system.model}
    </InfoLabel>
    <InfoLabel icon="fullscreen" title="Panel Resolution">
      {info?.display.width}x{info?.display.height}
    </InfoLabel>
    <InfoLabel icon="settings_ethernet" title="Mac Address">
      {info?.network.mac}
    </InfoLabel>
    <InfoLabel icon="wifi" title="IPv4 Address">
      {info?.network.ipv4}
    </InfoLabel>
    <InfoLabel icon="photo" title="Number of Photos">
      {new Intl.NumberFormat("en-US").format(info?.storage?.photos ?? 0)}
    </InfoLabel>
    <UsageBar
      used={info?.storage?.used ?? 0}
      total={info?.storage?.total ?? 0}
    />
  </Container>
</main>
