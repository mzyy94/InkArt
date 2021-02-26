<script lang="ts">
  import { Card, Button, Image, Dialog } from "smelte";

  function hideFile() {
    fetch(`/photos/${data.filename}`, {
      method: "PUT",
      body: JSON.stringify({ hide: !data.hidden }),
    }).then((res) => {
      if (res.ok) {
        data.hidden = !data.hidden;
      } else {
        // TODO: Show error snackbar
      }
    });
  }

  let showDialog = false;

  function deleteFile() {
    showDialog = false;
    fetch(`/photos/${data.filename}`, { method: "DELETE" }).then((res) => {
      if (res.ok) {
        // TODO: Remove card
      } else {
        // TODO: Show error snackbar
      }
    });
  }

  const replace = { flex: "inline-flex" };
  export let data: { filename: string; date: string; hidden: boolean };
</script>

<article class="m-3 md:w-2/5 sm:w-4/5">
  <Card.Card class={data.hidden ? "bg-gray-200" : ""}>
    <div slot="title">
      <Card.Title
        title={data.filename}
        subheader={new Date(data.date).toLocaleString()}
      />
    </div>
    <div slot="media">
      <Image
        class="w-full"
        src={`/photos/${data.filename}`}
        alt={data.filename}
      />
    </div>

    <div slot="actions">
      <div class="p-2">
        <Button
          icon={data.hidden ? "image" : "hide_image"}
          on:click={hideFile}
          text
          {replace}
        >
          {data.hidden ? "Show" : "Hide"}
        </Button>
        <Button
          color="error"
          icon="delete"
          on:click={() => (showDialog = true)}
          text
          {replace}
        >
          Delete
        </Button>
      </div>
    </div>
  </Card.Card>
</article>

<Dialog bind:value={showDialog}>
  <h5 slot="title">Delete file?</h5>
  <div class="text-gray-700">Are you sure you want to delete file?</div>
  <div slot="actions">
    <Button text on:click={() => (showDialog = false)}>Cancel</Button>
    <Button text on:click={deleteFile} color="error">Delete</Button>
  </div>
</Dialog>

<style>
  article {
    min-width: 240px;
  }
</style>
