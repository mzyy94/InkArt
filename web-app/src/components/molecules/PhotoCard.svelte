<script lang="ts">
  import { createEventDispatcher } from "svelte";
  import type { Entry } from "../../api";

  const dispatch = createEventDispatcher();

  function hideFile() {
    dispatch("hide", { data });
  }

  function deleteFile() {
    dispatch("delete", { data });
  }

  export let data: Entry;
</script>

<article>
  <header>
    {data.filename}
  </header>
  <img src={`/api/v1/photos/${data.filename}`} alt={data.filename} />
  <footer>
    <button class:secondary={!data.hidden} on:click={hideFile}>
      <i class="material-icons">{data.hidden ? "image" : "hide_image"}</i>
      {data.hidden ? "Show" : "Hide"}
    </button>
    <button class="contrast" on:click={deleteFile}>
      <i class="material-icons">delete</i>
      Delete
    </button>
  </footer>
</article>

<style>
  i {
    vertical-align: top;
  }
  article {
    margin-top: 0;
    margin-bottom: 20px;
  }
  header {
    padding: 1em;
  }
  footer {
    display: flex;
    flex-direction: row;
    justify-content: center;
    column-gap: 10px;
    padding: 24px;
  }
</style>
