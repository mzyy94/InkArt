<script lang="ts">
  import { Select } from "smelte";
  import ImageLoader from "../atoms/ImageLoader.svelte";
  import Canvas from "../atoms/Canvas.svelte";
  import Move from "../atoms/Move.svelte";

  const modes = [
    { value: "fit", text: "Fit" },
    { value: "fill", text: "Fill" },
    { value: "cover", text: "Cover" },
    { value: "none", text: "none" },
  ];

  const label = "Fitting mode";
  let mode = modes[0].value;

  const img = new Image();
</script>

<ImageLoader {img} />
<Select {label} items={modes} bind:value={mode} />
<Move let:offsetX let:offsetY reset={mode}>
  <Canvas {img} {mode} {offsetX} {offsetY} />
</Move>
