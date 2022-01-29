<script lang="ts">
  import { onMount } from "svelte";
  import api from "../../api";
  import type { Entry } from "../../api";
  import Container from "../templates/Container.svelte";
  import PhotoList from "../molecules/PhotoList.svelte";
  import PhotoGrid from "../molecules/PhotoGrid.svelte";
  import Snackbar from "../atoms/Snackbar.svelte";

  let list = false;

  let data: Entry[] = [];
  let loading = true;

  onMount(() => {
    loading = true;
    api
      .photos()
      .then((body) => {
        data = body.data;
      })
      .finally(() => {
        loading = false;
      });
  });

  function hideFile({ detail }: CustomEvent<{ data: Entry }>) {
    api
      .photos({
        data: [{ filename: detail.data.filename, hidden: !detail.data.hidden }],
      })
      .then((res) => {
        if (res.ok) {
          data = data.map((entry) =>
            entry.filename == detail.data.filename
              ? { ...entry, hidden: !entry.hidden }
              : entry
          );
        } else {
          snackbar.text = "Hide file failed";
          snackbar.error = true;
          snackbar.show = true;
        }
      });
  }

  let fileToDelete: Entry | null = null;

  function confirmDelete({ detail: { data } }: CustomEvent<{ data: Entry }>) {
    fileToDelete = data;
  }

  function deleteFile() {
    const filename = fileToDelete?.filename;
    fileToDelete = null;
    fetch(`/api/v1/photos/${filename}`, { method: "DELETE" }).then((res) => {
      if (res.ok) {
        data = data.filter((entry) => entry.filename != filename);
      } else {
        snackbar.text = "Delete file failed";
        snackbar.error = true;
        snackbar.show = true;
      }
    });
  }

  let snackbar = {
    show: false,
    text: "",
    error: false,
  };
</script>

<main>
  <Container>
    <span slot="title">File Management</span>
    <nav>
      <span><i class="material-icons">grid_view</i>Grid</span>
      <input type="checkbox" bind:checked={list} role="switch" />
      <span><i class="material-icons">list</i>List</span>
    </nav>
    <section>
      {#if list}
        <PhotoList
          bind:data
          bind:loading
          on:hide={hideFile}
          on:delete={confirmDelete}
        />
      {:else}
        <PhotoGrid
          bind:data
          bind:loading
          on:hide={hideFile}
          on:delete={confirmDelete}
        />
      {/if}
    </section>
  </Container>
</main>

<dialog open={fileToDelete != null}>
  <article>
    <header>Delete file?</header>
    <div>Are you sure you want to delete file?</div>
    <footer>
      <button class="secondary" on:click={() => (fileToDelete = null)}>
        Cancel
      </button>
      <button class="contrast" on:click={deleteFile}>Delete</button>
    </footer>
  </article>
</dialog>

<Snackbar error={snackbar.error} bind:open={snackbar.show}>
  <div>{snackbar.text}</div>
</Snackbar>

<style>
  i {
    vertical-align: top;
  }
  nav {
    margin-bottom: 2em;
  }
</style>
