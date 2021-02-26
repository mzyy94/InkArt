<script lang="ts">
  import { createEventDispatcher } from "svelte";
  import { Card, Button, Image } from "smelte";

  const dispatch = createEventDispatcher();

  function hideFile() {
    dispatch("hide", { data });
  }

  function deleteFile() {
    dispatch("delete", { data });
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
          on:click={deleteFile}
          text
          {replace}
        >
          Delete
        </Button>
      </div>
    </div>
  </Card.Card>
</article>

<style>
  article {
    min-width: 240px;
  }
</style>
