<script lang="ts">
  export let img: HTMLImageElement;
  export let mode = "fill";

  let canvas: HTMLCanvasElement;

  function drawImage() {
    if (!img.src) {
      return;
    }
    const ctx = canvas.getContext("2d");
    ctx.fillStyle = "white";
    ctx.fillRect(0, 0, canvas.width, canvas.height);
    switch (mode) {
      case "fill": {
        ctx.drawImage(img, 0, 0, canvas.width, canvas.height);
        break;
      }
      case "fit": {
        let x = 0;
        let y = 0;
        let { width, height } = canvas;

        const imageAspect = img.width / img.height;
        const canvasAspect = canvas.width / canvas.height;
        const scaleFactor = imageAspect / canvasAspect;

        if (scaleFactor < 1) {
          width = width * scaleFactor;
          x = (canvas.width - width) / 2;
        } else {
          height = height / scaleFactor;
          y = (canvas.height - height) / 2;
        }

        ctx.drawImage(img, x, y, width, height);
        break;
      }
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
