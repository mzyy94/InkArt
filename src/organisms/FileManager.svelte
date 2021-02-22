<script lang="ts">
  import { onMount } from "svelte";
  import PhotoList from "../molecules/PhotoList.svelte";

  let data: { filename: string; date: string; data: string }[] = [];
  let loading = true;

  onMount(() => {
    loading = true;
    fetch("/photos.json")
      .then((res) => res.json())
      .then((body) => {
        data = body.data;
      })
      .finally(() => {
        loading = false;
      });
  });

  let clazz = "";
  export { clazz as class };
</script>

<section class={clazz}>
  <PhotoList bind:data bind:loading />
</section>
