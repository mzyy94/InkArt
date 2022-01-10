<script lang="ts">
  import { createEventDispatcher } from "svelte";
  import { Card, Button, Image } from "smelte";
  import type { Entry } from "../../api";

  const dispatch = createEventDispatcher();

  function hideFile() {
    dispatch("hide", { data });
  }

  function deleteFile() {
    dispatch("delete", { data });
  }

  const replace = { flex: "inline-flex" };
  export let data: Entry;
</script>

<article class="m-6">
  <Card.Card class={data.hidden ? "bg-gray-200" : ""}>
    <div slot="title">
      <Card.Title title={data.filename} />
    </div>
    <div slot="media">
      <Image
        class={`w-full duration-200 ease-in ${data.hidden ? "opacity-60" : ""}`}
        src={`/api/v1/photos/${data.filename}`}
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
