<script lang="ts">
  import InfoLabel from "../atoms/InfoLabel.svelte";

  export let used: number;
  export let total: number;
  $: usage = used / total;

  function toByteString(bytes: number) {
    const units = ["byte", "kilobyte", "megabyte", "gigabyte", "terabyte"];
    const exponent = (Math.log2(bytes) / 10) | 0;
    const value = bytes / Math.pow(1024, exponent);
    return new Intl.NumberFormat(undefined, {
      style: "unit",
      unit: units[exponent],
    }).format(value);
  }
</script>

<InfoLabel icon="sd_card" title="SD Card Usage">
  {new Intl.NumberFormat(undefined, { style: "percent" }).format(usage)}
  ({toByteString(used)}/{toByteString(total)})
</InfoLabel>
<progress value={isNaN(usage) ? 0.01 : usage} />
