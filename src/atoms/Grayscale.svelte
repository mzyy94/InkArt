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

  class BmpDataView extends DataView {
    private pos = 0;
    public readonly width;
    public readonly height;
    public static readonly bytesPerPixel = 4;
    public static readonly headerSize = 122;

    static createArrayBuffer(width: number, height: number) {
      return new ArrayBuffer(
        width * height * BmpDataView.bytesPerPixel + BmpDataView.headerSize
      );
    }

    constructor(buffer: ArrayBufferLike, width: number, height: number) {
      super(buffer);
      this.width = width;
      this.height = height;
    }

    get imageSize() {
      return this.width * this.height * BmpDataView.bytesPerPixel;
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

    writePixel(data: number) {
      this.setUint32(this.pos, data, false);
      this.pos += 4;
    }
  }

  function imageData2Bmp({ data, width, height }: ImageData) {
    const imageData = new Uint32Array(data.buffer);
    const arrayBuffer = BmpDataView.createArrayBuffer(width, height);
    const bmp = new BmpDataView(arrayBuffer, width, height);

    // BITMAPFILEHEADER
    bmp.writeWord(0x4d42); // bfType - "BM"
    bmp.writeDWord(bmp.fileSize); // bfSize
    bmp.writeWord(); // bfReserved1
    bmp.writeWord(); // bfReserved2
    bmp.writeDWord(0x42); // bfOffBits

    // BITMAPINFOHEADER
    bmp.writeDWord(40); // biSize
    bmp.writeLong(width); // biWidth
    bmp.writeLong(-height >>> 0); // biHeight - negative height
    bmp.writeWord(1); // biPlanes - 1 plane
    bmp.writeWord(32); // biBitCount - 32-bits (BGRA)
    bmp.writeDWord(3); // biCompression - BI_BITFIELD = 3
    bmp.writeDWord(bmp.imageSize); // biSizeImage
    bmp.writeLong(2835); // biXPelsPerMeter
    bmp.writeLong(2835); // biYPelsPerMeter
    bmp.writeDWord(0); // biClrUsed
    bmp.writeDWord(0); // biClrImportant

    // RGBQUAD
    bmp.writeDWord(0x00ff0000); // RedMask
    bmp.writeDWord(0x0000ff00); // GreenMask
    bmp.writeDWord(0x000000ff); // BlueMask

    for (let y = 0; y < height; y++) {
      for (let x = 0; x < width; x++) {
        const abgr = imageData[x + y * width];
        const a = abgr >>> 24;
        bmp.writePixel((abgr << 8) | a);
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

<canvas bind:this={canvas} {width} {height} />

<style>
  canvas {
    max-width: 100%;
    margin: 1rem auto;
    border: 2px solid gray;
  }
</style>
