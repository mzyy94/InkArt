<script lang="ts">
  import { ProgressLinear } from "smelte";
  import PhotoCard from "./PhotoCard.svelte";

  export let data: { filename: string; date: string; hidden: boolean }[];
  export let loading: boolean;
</script>

<section class="flex w-full flex-row flex-wrap justify-center">
  {#if loading}
    <ProgressLinear />
  {/if}
  {#each data as elm}
    <PhotoCard data={elm} on:delete on:hide />
  {/each}
</section>
