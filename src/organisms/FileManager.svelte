<script lang="ts">
  import { DataTable } from "smelte";
  import { onMount } from "svelte";

  let data: { filename: string; date: string; data: string }[] = [];
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

  let clazz = "";
  export { clazz as class };
</script>

<section class={clazz}>
  <DataTable
    {data}
    {loading}
    columns={[
      { label: "Name", field: "filename", editable: false },
      {
        label: "Date",
        value: ({ date }) => new Date(date).toLocaleString(),
        editable: false,
      },
      {
        field: "Image",
        value: (v) => `<img src="${v.data}" height="70" alt="${v.filename}">`,
        class: "w-64",
        sortable: false,
        editable: false,
        headerRemove: "justify-end",
      },
    ]}
  />
</section>
