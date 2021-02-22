<script lang="ts">
  import { Select } from "smelte";
  import ImageLoader from "../atoms/ImageLoader.svelte";
  import Grayscale from "../atoms/Grayscale.svelte";
  import Move from "../atoms/Move.svelte";
  import ProgressButton from "../atoms/ProgressButton.svelte";

  const modes = [
    { value: "fit", text: "Fit" },
    { value: "fill", text: "Fill" },
    { value: "cover", text: "Cover" },
    { value: "none", text: "none" },
  ];

  const label = "Fitting mode";
  let mode = modes[0].value;

  const img = new Image();
  let active = false;
  let grayscale: Grayscale;
  let uploading = false;

  function uploadImage() {
    uploading = true;
    const png = grayscale.getPngDataURL();
    fetch("/photos.json", { method: "POST", body: png })
      .then((res) => res.json())
      .then(console.log)
      .finally(() => {
        uploading = false;
      });
  }
</script>

<ImageLoader {img} />
<Select {label} items={modes} bind:value={mode} />
<Move let:offsetX let:offsetY reset={mode} {active}>
  <Grayscale
    bind:this={grayscale}
    {img}
    {mode}
    {offsetX}
    {offsetY}
    bind:active
  />
</Move>

<ProgressButton on:click={uploadImage} loading={uploading}>
  Upload
  <span slot="loading">Uploading...</span>
</ProgressButton>
