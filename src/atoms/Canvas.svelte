<script lang="ts">
  export let img: HTMLImageElement;
  export let mode = "fill";

  let canvas: HTMLCanvasElement;

  function drawImage() {
    if (!img.src) {
      return;
    }
    const ctx = canvas.getContext("2d");
    if (mode == "fill") {
      ctx.drawImage(img, 0, 0, canvas.width, canvas.height);
    }
  }

  img.onload = drawImage;
  $: {
    mode;
    drawImage();
  }
</script>

<canvas bind:this={canvas} width={800} height={600} />

<style>
  canvas {
    background: gray;
    max-width: 100%;
  }
</style>
