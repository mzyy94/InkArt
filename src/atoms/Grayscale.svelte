<script context="module" lang="ts">
  const vertShader = `
  attribute vec2 position;
  uniform mat3 matrix;
  varying vec2 vTexCoord;

  void main() {
    gl_Position = vec4(matrix * vec3(position, 1), 1);
    vTexCoord = position;
  }`;

  const fragShader = `
  precision mediump float;

  uniform sampler2D texture;
  varying vec2 vTexCoord;

  // ITU-R Rec BT.601
  const vec3 filter = vec3(0.299, 0.587, 0.114);
  const float depth = 8.0;

  void main() {
      vec4 color = texture2D(texture, vTexCoord);
      float gray = dot(color.rgb, filter);
      gl_FragColor = vec4(floor(gray * vec3(depth)) / vec3(depth), 1.0);
  }`;

  function createShader(
    gl: WebGLRenderingContext,
    script: string,
    type: number
  ) {
    const shader = gl.createShader(type)!;
    gl.shaderSource(shader, script);
    gl.compileShader(shader);

    const compiled = gl.getShaderParameter(shader, gl.COMPILE_STATUS);

    if (!compiled) {
      const lastError = gl.getShaderInfoLog(shader);
      console.error("Error compiling shader '" + shader + "':" + lastError);
      gl.deleteShader(shader);
      return shader;
    }
    return shader;
  }

  function createProgram(
    gl: WebGLRenderingContext,
    vertexScript: string,
    fragScript: string
  ) {
    const vertexShader = createShader(gl, vertexScript, gl.VERTEX_SHADER);
    const fragShader = createShader(gl, fragScript, gl.FRAGMENT_SHADER);

    const program = gl.createProgram()!;
    gl.attachShader(program, vertexShader);
    gl.attachShader(program, fragShader);

    gl.linkProgram(program);

    const linked = gl.getProgramParameter(program, gl.LINK_STATUS);
    if (!linked) {
      const error = gl.getProgramInfoLog(program);
      console.error("Failed to link program:", error);
      gl.deleteProgram(program);
      return program;
    }

    const positionLocation = gl.getAttribLocation(program, "position");

    const buffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
    gl.bufferData(
      gl.ARRAY_BUFFER,
      new Float32Array([0, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 1]),
      gl.STATIC_DRAW
    );
    gl.enableVertexAttribArray(positionLocation);
    gl.vertexAttribPointer(positionLocation, 2, gl.FLOAT, false, 0, 0);

    return program;
  }

  function setTexture(
    gl: WebGLRenderingContext,
    texture: WebGLTexture,
    source: TexImageSource
  ) {
    gl.bindTexture(gl.TEXTURE_2D, texture);

    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);

    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, source);
  }
</script>

<script lang="ts">
  import { onMount } from "svelte";

  export let img: HTMLImageElement;
  export let mode = "fill";

  export let offsetX = 0;
  export let offsetY = 0;
  export let active = false;
  export let algorithm = "default";

  export let width = 800;
  export let height = 600;

  function resizeImage(mode: string, x = 0, y = 0) {
    if (!img.src) {
      return;
    }

    const resized = document.createElement("canvas");
    resized.width = canvas.width;
    resized.height = canvas.height;

    const ctx = resized.getContext("2d")!;
    let { width, height } = ctx.canvas;

    ctx.fillStyle = "white";
    ctx.fillRect(0, 0, width, height);

    switch (mode) {
      case "cover":
      case "fit": {
        const imageAspect = img.width / img.height;
        const canvasAspect = ctx.canvas.width / ctx.canvas.height;
        const scaleFactor = imageAspect / canvasAspect;
        const threshold = mode == "cover" ? scaleFactor : 1 / scaleFactor;
        if (threshold > 1) {
          width *= scaleFactor;
          x += (ctx.canvas.width - width) / 2;
        } else {
          height /= scaleFactor;
          y += (ctx.canvas.height - height) / 2;
        }
        break;
      }
      case "none": {
        x += (width - img.width) / 2;
        y += (height - img.height) / 2;
        width = img.width;
        height = img.height;
        break;
      }
      case "fill":
      default: {
        break;
      }
    }

    ctx.drawImage(img, x, y, width, height);
    setTexture(gl, texture, resized);
  }

  let canvas: HTMLCanvasElement;
  let gl: WebGLRenderingContext;
  let program: WebGLProgram;
  let texture: WebGLTexture;

  onMount(() => {
    gl = canvas.getContext("webgl", { preserveDrawingBuffer: true })!;
    program = createProgram(gl, vertShader, fragShader);
    texture = gl.createTexture()!;
  });

  function drawImage(x: number, y: number) {
    if (!gl) {
      return;
    }

    if (mode == "cover" || mode == "none") {
      resizeImage(mode, x, y);
      x = y = 0;
    }

    if (algorithm != "default") {
      return;
    }

    gl.useProgram(program);
    gl.clearColor(1, 1, 1, 1);
    gl.clearDepth(1);
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    const matrixLocation = gl.getUniformLocation(program, "matrix");
    const xx = (x / gl.canvas.width) * 2 - 1;
    const yy = (y / gl.canvas.height) * -2 + 1;
    gl.uniformMatrix3fv(matrixLocation, false, [
      ...[2, 0, 0],
      ...[0, -2, 0],
      ...[xx, yy, 1],
    ]);

    gl.drawArrays(gl.TRIANGLES, 0, 6);
  }

  img.onload = () => {
    resizeImage(mode);
    drawImage(offsetX, offsetY);
    active = true;
  };

  $: {
    resizeImage(mode);
    drawImage(0, 0);
  }
  $: drawImage(offsetX, offsetY);

  export function getPngDataURL() {
    return canvas.toDataURL("image/png");
  }
</script>

<canvas bind:this={canvas} {width} {height} />

<style>
  canvas {
    max-width: 100%;
    margin: 1rem auto;
    border: 2px solid gray;
  }
</style>
