<script lang="ts">
  import { Select, Snackbar } from "smelte";
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

  let snackbar = {
    show: false,
    text: "",
    color: "primary",
  };

  let grayscale: Grayscale;
  let uploading = false;

  function uploadImage() {
    uploading = true;
    const bmp = grayscale.getBmpArrayBuffer();
    const blob = new Blob([bmp], { type: "image/bmp" });
    const formData = new FormData();
    formData.append("file", blob, `image-${Date.now()}.bmp`);
    fetch("/upload", { method: "PUT", body: formData })
      .then((res) => {
        snackbar.text = `Upload ${res.ok ? "succeeded" : "failed"}`;
        snackbar.color = res.ok ? "primary" : "error";
        snackbar.show = true;
      })
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

<Snackbar color={snackbar.color} bind:value={snackbar.show}>
  <div>{snackbar.text}</div>
</Snackbar>
