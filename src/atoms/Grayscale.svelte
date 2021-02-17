<script context="module" lang="ts">
  export function grayscale(node: Node, algorithm = "default") {
    node.addEventListener("drawimage", ({ target }: CustomEvent) => {
      if (algorithm != "default") {
        return;
      }
      const canvas = target as HTMLCanvasElement;
      const ctx = canvas.getContext("2d");
      const image = ctx.getImageData(0, 0, canvas.width, canvas.height);
      const { data } = image;

      for (let i = 0; i < data.length; i += 4) {
        const luminance =
          data[i] * 0.299 + data[i + 1] * 0.587 + data[i + 2] * 0.114;
        data.fill(luminance, i, i + 3);
      }

      ctx.putImageData(image, 0, 0);
    });
  }
</script>
