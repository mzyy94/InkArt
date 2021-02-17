<script lang="ts">
  export let img: HTMLImageElement;
  export let mode = "fill";

  let canvas: HTMLCanvasElement;

  function drawImage(offsetX = 0, offsetY = 0) {
    if (!img.src) {
      return;
    }
    const ctx = canvas.getContext("2d");
    let x = 0;
    let y = 0;
    let { width, height } = canvas;

    ctx.fillStyle = "white";
    ctx.fillRect(x, y, width, height);

    switch (mode) {
      case "cover":
      case "fit": {
        const imageAspect = img.width / img.height;
        const canvasAspect = canvas.width / canvas.height;
        const scaleFactor = imageAspect / canvasAspect;
        const threshold = mode == "cover" ? scaleFactor : 1 / scaleFactor;
        if (threshold > 1) {
          width = width * scaleFactor;
          x = (canvas.width - width) / 2;
        } else {
          height = height / scaleFactor;
          y = (canvas.height - height) / 2;
        }
        break;
      }
      case "none": {
        x = (width - img.width) / 2;
        y = (height - img.height) / 2;
        width = img.width;
        height = img.height;
        break;
      }
      case "fill":
      default: {
        break;
      }
    }

    ctx.drawImage(img, x + offsetX, y + offsetY, width, height);
  }

  let offsetX = 0;
  let offsetY = 0;

  img.onload = () => {
    offsetX = offsetY = 0;
    drawImage();
  };

  $: {
    mode;
    offsetX = offsetY = 0;
    drawImage();
  }

  let pointer = { x: 0, y: 0, moving: false };

  function beginMove(event: PointerEvent) {
    pointer.x = event.x - offsetX;
    pointer.y = event.y - offsetY;
    pointer.moving = true;
  }

  function endMove() {
    pointer.moving = false;
  }

  function move(event: PointerEvent) {
    if (pointer.moving) {
      offsetX = event.x - pointer.x;
      offsetY = event.y - pointer.y;
      drawImage(offsetX, offsetY);
    }
  }
</script>

<canvas
  bind:this={canvas}
  class="cursor-move"
  on:pointerdown={beginMove}
  on:pointermove={move}
  on:pointerup={endMove}
  on:pointerout={endMove}
  on:pointercancel={endMove}
  width={800}
  height={600}
/>

<style>
  canvas {
    background: gray;
    max-width: 100%;
  }
</style>
