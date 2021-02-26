<script lang="ts">
  import { onMount } from "svelte";
  import { Tab, Tabs, Button, Dialog } from "smelte";
  import PhotoCard from "../molecules/PhotoCard.svelte";
  import PhotoList from "../molecules/PhotoList.svelte";

  let mode = "list";

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
        // TODO: Toggle visibility
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
    fetch(`/photos/${fileToDelete?.filename}`, { method: "DELETE" })
      .then((res) => {
        if (res.ok) {
          // TODO: Remove entry
        } else {
          // TODO: Show error snackbar
        }
      })
      .finally(() => {
        fileToDelete = null;
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
        <section class="flex w-full flex-row flex-wrap justify-center">
          {#each data as elm}
            <PhotoCard
              data={elm}
              on:delete={confirmDelete}
              on:hide={hideFile}
            />
          {/each}
        </section>
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
