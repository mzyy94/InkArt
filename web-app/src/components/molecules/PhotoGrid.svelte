<script lang="ts">
  import PhotoCard from "./PhotoCard.svelte";
  import type { Entry } from "../../api";

  export let data: Entry[];
  export let loading: boolean;
</script>

<section aria-busy={loading}>
  <div>
    {#each data as elm}
      <PhotoCard data={elm} on:delete on:hide />
    {/each}
  </div>
</section>

<style>
  div {
    display: grid;
    column-gap: 20px;
    grid-template-columns: repeat(1, minmax(0, 1fr));
  }
  @media (min-width: 768px) {
    div {
      grid-template-columns: repeat(2, minmax(0, 1fr));
    }
  }
</style>
