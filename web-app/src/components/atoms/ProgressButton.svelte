<script lang="ts">
  import { Button, ProgressCircular } from "smelte";
  export let loading = false;
  export let disabled = false;
</script>

<Button block on:click disabled={loading || disabled}>
  {#if loading}
    <span class="inline-block align-text-bottom">
      <ProgressCircular size={16} width={2} color="gray" />
    </span>
    <slot name="loading">Loading...</slot>
  {:else}
    <slot />
  {/if}
</Button>
