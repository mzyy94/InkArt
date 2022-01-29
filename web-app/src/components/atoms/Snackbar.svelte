<script lang="ts">
  export let open = false;
  export let error = false;

  $: if (open) {
    setTimeout(() => {
      open = false;
    }, 2500);
  }
</script>

{#if open}
  <div><span class:error><slot /></span></div>
{/if}

<style>
  div {
    z-index: 1;
    position: fixed;
    display: flex;
    bottom: 2em;
    align-items: center;
    flex-direction: row;
    justify-content: center;
    width: 100vw;
  }
  span {
    background-color: darkslategray;
    color: white;
    border-radius: 5px;
    padding: 0.5em 2em;
  }
  span.error {
    background-color: tomato;
  }
</style>
