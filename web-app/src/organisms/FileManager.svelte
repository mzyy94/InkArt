<script lang="ts">
  import { onMount } from "svelte";
  import { Tab, Tabs, Button, Dialog, Snackbar } from "smelte";
  import api from "../api";
  import type { Entry } from "../api";
  import Container from "../templates/Container.svelte";
  import PhotoList from "../molecules/PhotoList.svelte";
  import PhotoGrid from "../molecules/PhotoGrid.svelte";

  let mode = "grid";

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
          snackbar.color = "error";
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
    fetch(`/api/photos/${filename}`, { method: "DELETE" }).then((res) => {
      if (res.ok) {
        data = data.filter((entry) => entry.filename != filename);
      } else {
        snackbar.text = "Delete file failed";
        snackbar.color = "error";
        snackbar.show = true;
      }
    });
  }

  let snackbar = {
    show: false,
    text: "",
    color: "primary",
  };
</script>

<main>
  <Container>
    <span slot="title">File Management</span>
    <section>
      <Tabs
        bind:selected={mode}
        color={"black"}
        notSelectedColor="gray"
        indicator={false}
        items={[
          { id: "grid", text: "grid", icon: "grid_view" },
          { id: "list", text: "list", icon: "list" },
        ]}
      >
        <div slot="content">
          <Tab id="grid" selected={mode}>
            <PhotoGrid
              bind:data
              bind:loading
              on:hide={hideFile}
              on:delete={confirmDelete}
            />
          </Tab>
          <Tab id="list" selected={mode}>
            <PhotoList
              bind:data
              bind:loading
              on:hide={hideFile}
              on:delete={confirmDelete}
            />
          </Tab>
        </div>
      </Tabs>
    </section>
  </Container>
</main>

<Dialog value={fileToDelete != null}>
  <h5 slot="title">Delete file?</h5>
  <div class="text-gray-700">Are you sure you want to delete file?</div>
  <div slot="actions">
    <Button text on:click={() => (fileToDelete = null)}>Cancel</Button>
    <Button text on:click={deleteFile} color="error">Delete</Button>
  </div>
</Dialog>

<Snackbar color={snackbar.color} bind:value={snackbar.show}>
  <div>{snackbar.text}</div>
</Snackbar>
