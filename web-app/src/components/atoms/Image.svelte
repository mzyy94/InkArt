<script lang="ts">
  import { useLazyImage as lazyImage } from "svelte-lazy-image";
  import { open } from "../../actions/dialog";

  export let src: string;
  $: alt = src.split("/").pop();

  let width = 0;
  let height = 0;

  function getSize(event: Event) {
    const img = event.target as HTMLImageElement;
    width = img.naturalWidth;
    height = img.naturalHeight;
  }

  let modal = false;
  const showModal = () => (modal = true);
  const hideModal = () => (modal = false);
</script>

<div data-tooltip={`${width}x${height}`}>
  <img
    data-src={src}
    {alt}
    on:click={showModal}
    on:load={getSize}
    use:lazyImage
  />
</div>

<dialog use:open={modal} on:click={hideModal}>
  <img {src} {alt} />
</dialog>

<style>
  div > img {
    cursor: pointer;
  }
</style>
