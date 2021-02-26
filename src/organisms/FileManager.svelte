<script lang="ts">
  import { onMount } from "svelte";
  import { Tab, Tabs, Button, Dialog } from "smelte";
  import PhotoList from "../molecules/PhotoList.svelte";
  import PhotoGrid from "../molecules/PhotoGrid.svelte";

  let mode = "grid";

  type Entry = { filename: string; date: string; hidden: boolean };
  let data: Entry[] = [];
  let loading = true;

  onMount(() => {
    loading = true;
    fetch("/photos.json")
      .then((res) => res.json())
      .then((body) => {
        data = body.data;
      })
      .finally(() => {
        loading = false;
      });
  });

  function hideFile({ detail }: CustomEvent<{ data: Entry }>) {
    fetch(`/photos/${detail.data.filename}`, {
      method: "PUT",
      body: JSON.stringify({ hide: !detail.data.hidden }),
    }).then((res) => {
      if (res.ok) {
        data = data.map((entry) =>
          entry.filename == detail.data.filename
            ? { ...entry, hidden: !entry.hidden }
            : entry
        );
      } else {
        // TODO: Show error snackbar
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
    fetch(`/photos/${filename}`, { method: "DELETE" }).then((res) => {
      if (res.ok) {
        data = data.filter((entry) => entry.filename != filename);
      } else {
        // TODO: Show error snackbar
      }
    });
  }

  let clazz = "";
  export { clazz as class };
</script>

<section class={clazz}>
  <Tabs
    bind:selected={mode}
    color="black"
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
        <PhotoList bind:data bind:loading />
      </Tab>
    </div>
  </Tabs>
</section>

<Dialog value={fileToDelete != null}>
  <h5 slot="title">Delete file?</h5>
  <div class="text-gray-700">Are you sure you want to delete file?</div>
  <div slot="actions">
    <Button text on:click={() => (fileToDelete = null)}>Cancel</Button>
    <Button text on:click={deleteFile} color="error">Delete</Button>
  </div>
</Dialog>
