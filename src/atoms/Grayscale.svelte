<script context="module" lang="ts">
  import type { DrawImageEvent } from "./Canvas.svelte";
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

  void main() {
      vec4 color = texture2D(texture, vTexCoord);
      float gray = dot(color.rgb, filter);
      gl_FragColor = vec4(vec3(gray), 1.0);
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

  export function grayscale(node: HTMLCanvasElement, algorithm = "default") {
    const canvas = document.createElement("canvas");
    canvas.width = node.width;
    canvas.height = node.height;
    const gl = canvas.getContext("webgl");
    const program = createProgram(gl, vertShader, fragShader);
    const texture = gl.createTexture();

    node.addEventListener("drawimage", (e: DrawImageEvent) => {
      if (algorithm != "default") {
        return;
      }
      const targetCanvas = e.target as HTMLCanvasElement;
      const { image, x: xx, y: yy } = e.detail;

      gl.useProgram(program);
      gl.clearColor(1, 1, 1, 1);
      gl.clearDepth(1);
      gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
      setTexture(gl, texture, image);

      const matrixLocation = gl.getUniformLocation(program, "matrix");
      const x = (xx / gl.canvas.width) * 2 - 1;
      const y = (yy / gl.canvas.height) * -2 + 1;
      gl.uniformMatrix3fv(matrixLocation, false, [
        ...[2, 0, 0],
        ...[0, -2, 0],
        ...[x, y, 1],
      ]);

      gl.drawArrays(gl.TRIANGLES, 0, 6);

      const ctx = targetCanvas.getContext("2d");
      ctx.drawImage(canvas, 0, 0);
    });
  }
</script>
