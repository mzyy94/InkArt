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
  const float depth = 4.0;

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
  import { onMount, tick } from "svelte";
  import api from "../../api";

  export let img: HTMLImageElement;
  export let mode = "fill";

  export let offsetX = 0;
  export let offsetY = 0;
  export let active = false;
  export let algorithm = "default";

  let width = 0;
  let height = 0;

  function resizeImageRect(
    img: HTMLImageElement,
    mode: string,
    x: number,
    y: number
  ) {
    let { width, height } = canvas;

    switch (mode) {
      case "cover":
      case "fit": {
        const imageAspect = img.width / img.height;
        const canvasAspect = canvas.width / canvas.height;
        const scaleFactor = imageAspect / canvasAspect;
        const threshold = mode == "cover" ? scaleFactor : 1 / scaleFactor;
        if (scaleFactor == 1) {
          x = y = 0;
        } else if (threshold > 1) {
          width *= scaleFactor;
          x += (canvas.width - width) / 2;
          y = 0;
        } else {
          height /= scaleFactor;
          y += (canvas.height - height) / 2;
          x = 0;
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
        x = y = 0;
        break;
      }
    }

    return { x, y, width, height };
  }

  let canvas: HTMLCanvasElement;
  let gl: WebGLRenderingContext;
  let program: WebGLProgram;
  let texture: WebGLTexture;

  onMount(async () => {
    const { display } = await api.info();
    const { padding, orientation } = await api.display();
    width = display.width - (padding.left + padding.right);
    height = display.height - (padding.top + padding.bottom);
    if (orientation.startsWith("portrait")) {
      [width, height] = [height, width];
    }
    await tick();
    gl = canvas.getContext("webgl", { preserveDrawingBuffer: true })!;
    program = createProgram(gl, vertShader, fragShader);
    texture = gl.createTexture()!;
  });

  function drawImage(mode: string, x: number, y: number) {
    if (!gl || !img.src) {
      return;
    }
    const resized = resizeImageRect(img, mode, x, y);

    x = resized.x;
    y = resized.y;
    const ww = (2 * resized.width) / gl.canvas.width;
    const hh = (-2 * resized.height) / gl.canvas.height;

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
      ...[ww, 0, 0],
      ...[0, hh, 0],
      ...[xx, yy, 1],
    ]);

    gl.drawArrays(gl.TRIANGLES, 0, 6);
  }

  img.onload = () => {
    setTexture(gl, texture, img);
    drawImage(mode, 0, 0);
    active = true;
  };

  $: drawImage(mode, offsetX, offsetY);

  class BmpDataView extends DataView {
    private pos = 0;
    public readonly width;
    public readonly height;
    public static readonly bytesPerPixel = 1 / 2;
    public static readonly headerSize = 118;

    static createArrayBuffer(width: number, height: number) {
      let size = width * height;
      if (width % 8) {
        size += height * (8 - (width % 8));
      }
      return new ArrayBuffer(
        size * BmpDataView.bytesPerPixel + BmpDataView.headerSize
      );
    }

    constructor(buffer: ArrayBufferLike, width: number, height: number) {
      super(buffer);
      this.width = width;
      this.height = height;
    }

    get imageSize() {
      let size = width * height;
      if (width % 8) {
        size += height * (8 - (width % 8));
      }
      return size * BmpDataView.bytesPerPixel;
    }

    get fileSize() {
      return this.imageSize + BmpDataView.headerSize;
    }

    writeWord(data = 0) {
      this.setUint16(this.pos, data, true);
      this.pos += 2;
    }

    writeDWord(data = 0) {
      this.setUint32(this.pos, data, true);
      this.pos += 4;
    }

    writeShort(data = 0) {
      this.writeWord(data);
    }

    writeLong(data = 0) {
      this.writeDWord(data);
    }
  }

  function imageData2Bmp({ data, width, height }: ImageData) {
    const imageData = new Uint8Array(data.buffer);
    const arrayBuffer = BmpDataView.createArrayBuffer(width, height);
    const bmp = new BmpDataView(arrayBuffer, width, height);

    // BITMAPFILEHEADER
    bmp.writeWord(0x4d42); // bfType - "BM"
    bmp.writeDWord(bmp.fileSize); // bfSize
    bmp.writeWord(); // bfReserved1
    bmp.writeWord(); // bfReserved2
    bmp.writeDWord(BmpDataView.headerSize); // bfOffBits

    // BITMAPINFOHEADER
    bmp.writeDWord(40); // biSize
    bmp.writeLong(width); // biWidth
    bmp.writeLong(height); // biHeight
    bmp.writeWord(1); // biPlanes - 1 plane
    bmp.writeWord(4); // biBitCount - 4-bits (16-Palettes)
    bmp.writeDWord(0); // biCompression - BI_RGB = 0
    bmp.writeDWord(bmp.imageSize); // biSizeImage
    bmp.writeLong(2835); // biXPelsPerMeter
    bmp.writeLong(2835); // biYPelsPerMeter
    bmp.writeDWord(8); // biClrUsed
    bmp.writeDWord(0); // biClrImportant

    // RGBQUAD
    for (let i = 0; i < 8; i++) {
      bmp.writeDWord(0x00222222 * i);
    }
    for (let i = 0; i < 8; i++) {
      bmp.writeDWord(0x00000000);
    }

    for (let y = height - 1; y >= 0; y--) {
      for (let x = 0; x < width; x += 8) {
        const i = y * width + x;
        let data = 0;
        for (let j = 0; j < 8; j++) {
          const pixel = imageData[(i + j) * 4] >> 5;
          const pos = j % 2 ? j - 1 : j + 1;
          data |= pixel << (4 * pos);
        }
        bmp.writeDWord(data);
      }
    }

    return arrayBuffer;
  }

  export function getBmpArrayBuffer() {
    const print = document.createElement("canvas");
    print.width = width;
    print.height = height;
    const ctx = print.getContext("2d")!;
    ctx.drawImage(canvas, 0, 0);
    const imageData = ctx.getImageData(0, 0, width, height);
    return imageData2Bmp(imageData);
  }
</script>

{#if width > 0 && height > 0}
  <span>Image size: {width}x{height}</span>
  <canvas bind:this={canvas} {width} {height} />
{/if}

<style>
  canvas {
    max-width: 100%;
    margin: 1rem auto;
    border: 2px solid gray;
  }
</style>
