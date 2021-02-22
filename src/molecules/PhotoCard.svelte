<script lang="ts">
  import { Card, Button, Image } from "smelte";

  const replace = { flex: "inline-flex" };
  export let data: { filename: string; date: string; data: string };
</script>

<article class="m-3 md:w-2/5 sm:w-4/5">
  <Card.Card>
    <div slot="title">
      <Card.Title
        title={data.filename}
        subheader={new Date(data.date).toLocaleString()}
      />
    </div>
    <div slot="media">
      <Image class="w-full" src={data.data} alt={data.filename} />
    </div>

    <div slot="actions">
      <div class="p-2">
        <Button icon="hide_image" text {replace}>Hide</Button>
        <Button color="error" icon="delete" text {replace}>Delete</Button>
      </div>
    </div>
  </Card.Card>
</article>

<style>
  article {
    min-width: 240px;
  }
</style>
