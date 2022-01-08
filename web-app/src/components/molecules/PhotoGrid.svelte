<script lang="ts">
  import { ProgressLinear } from "smelte";
  import PhotoCard from "./PhotoCard.svelte";
  import type { Entry } from "../../api";

  export let data: Entry[];
  export let loading: boolean;
</script>

<section class="w-full">
  {#if loading}
    <ProgressLinear />
  {/if}
  <div class="grid md:grid-cols-2 grid-cols-1">
    {#each data as elm}
      <PhotoCard data={elm} on:delete on:hide />
    {/each}
  </div>
</section>
