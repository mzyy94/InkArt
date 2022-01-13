<script lang="ts">
  import { ProgressLinear } from "smelte";
  import InfoLabel from "../atoms/InfoLabel.svelte";

  export let used: number;
  export let total: number;
  $: usage = used / total;

  function toByteString(bytes: number) {
    const labels = ["B", "KB", "MB", "GB", "TB"];
    const exponent = (Math.log2(bytes) / 10) | 0;
    const value = bytes / Math.pow(1024, exponent);
    return Intl.NumberFormat("en-US").format(value) + labels[exponent];
  }
</script>

<InfoLabel icon="sd_card" title="SD Card Usage">
  {new Intl.NumberFormat("en-US", { style: "percent" }).format(usage)}
  ({toByteString(used)}/{toByteString(total)})
</InfoLabel>
<div class="mx-3 mb-3 bg-gray-400 rounded overflow-hidden">
  <ProgressLinear progress={isNaN(usage) ? 0.01 : usage * 100} />
</div>
