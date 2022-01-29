<script lang="ts">
  import { Select, Slider } from "smelte";
  import Container from "../templates/Container.svelte";
  import ImageLoader from "../atoms/ImageLoader.svelte";
  import Grayscale from "../atoms/Grayscale.svelte";
  import Move from "../atoms/Move.svelte";
  import ProgressButton from "../atoms/ProgressButton.svelte";
  import Snackbar from "../atoms/Snackbar.svelte";

  const modes = [
    { value: "cover", text: "Cover" },
    { value: "fit", text: "Fit" },
    { value: "fill", text: "Fill" },
    { value: "none", text: "none" },
  ];

  const label = "Fitting mode";
  let mode = modes[0].value;

  const img = new Image();
  let active = false;
  let brightness = 1.0;

  let snackbar = {
    show: false,
    text: "",
    error: false,
  };

  let grayscale: Grayscale;
  let width = 0;
  let height = 0;
  let uploading = false;

  async function uploadImage() {
    uploading = true;
    const bmp = grayscale.getBmpArrayBuffer();
    const blob = new Blob([bmp], { type: "image/bmp" });
    const encodedText = await new Promise<string>((resolve, reject) => {
      const reader = new FileReader();
      reader.onload = () => {
        const result = reader.result as string;
        const offset = result.indexOf(",") + 1;
        resolve(result.slice(offset));
      };
      reader.onerror = () => reject(reader.error);
      reader.readAsDataURL(blob);
    });

    fetch("/api/v1/photos", { method: "POST", body: encodedText })
      .then((res) => {
        snackbar.text = `Upload ${res.ok ? "succeeded" : "failed"}`;
        snackbar.error = !res.ok;
        snackbar.show = true;
      })
      .finally(() => {
        uploading = false;
      });
  }
</script>

<main>
  <Container>
    <span slot="title">Upload Photo</span>
    <ImageLoader {img} />
    <Select {label} items={modes} bind:value={mode} />
    <span>Image size: {width}x{height}</span>
    <Move let:offsetX let:offsetY reset={mode} {active}>
      <Grayscale
        bind:this={grayscale}
        {img}
        {mode}
        {offsetX}
        {offsetY}
        {brightness}
        bind:width
        bind:height
        bind:active
      />
    </Move>

    <fieldset class="my-3">
      <p class="text-gray-700">Brightness: {brightness.toFixed(2)}</p>
      <Slider
        min={0}
        max={3}
        default={1}
        step={0.05}
        disabled={!active}
        bind:value={brightness}
      />
    </fieldset>

    <ProgressButton
      on:click={uploadImage}
      disabled={!active}
      loading={uploading}
    >
      Upload
      <span slot="loading">Uploading...</span>
    </ProgressButton>
  </Container>
</main>

<Snackbar error={snackbar.error} bind:open={snackbar.show}>
  <div>{snackbar.text}</div>
</Snackbar>
