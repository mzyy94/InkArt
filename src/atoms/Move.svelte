<script lang="ts">
  export let reset: any;
  export let active: boolean;

  let pointer = { x: 0, y: 0, moving: false };
  let offsetX = 0;
  let offsetY = 0;

  $: {
    reset;
    offsetX = offsetY = 0;
  }

  function beginMove(event: PointerEvent) {
    pointer.x = event.x - offsetX;
    pointer.y = event.y - offsetY;
    pointer.moving = active;
  }

  function endMove() {
    pointer.moving = false;
  }

  function move(event: PointerEvent) {
    if (pointer.moving) {
      offsetX = event.x - pointer.x;
      offsetY = event.y - pointer.y;
    }
  }
</script>

<div
  class:cursor-move={active}
  on:pointerdown={beginMove}
  on:pointermove={move}
  on:pointerup={endMove}
  on:pointerout={endMove}
  on:pointercancel={endMove}
  on:touchmove|preventDefault
>
  <slot {offsetX} {offsetY} />
</div>
