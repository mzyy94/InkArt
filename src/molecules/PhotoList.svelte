<script lang="ts">
  import { DataTable } from "smelte";

  export let data: { filename: string; date: string; data: string }[];
  export let loading: boolean;
</script>

<section class="mt-3">
  <DataTable
    editable={false}
    {data}
    {loading}
    columns={[
      { label: "Name", field: "filename", editable: false },
      {
        label: "Date",
        value: ({ date }) => new Date(date).toLocaleString(),
        headerRemove: "justify-end",
      },
      {
        field: "Image",
        value: (v) => `<img src="${v.data}" height="70" alt="${v.filename}">`,
        class: "w-64",
        sortable: false,
        headerRemove: "justify-end",
      },
    ]}
  />
</section>
