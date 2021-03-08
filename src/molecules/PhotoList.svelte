<script lang="ts">
  import { DataTable } from "smelte";

  export let data: { filename: string; date: string; hidden: boolean }[];
  export let loading: boolean;
</script>

<section class="mt-3">
  <DataTable
    class="w-full"
    editable={false}
    {data}
    {loading}
    columns={[
      { label: "Name", field: "filename" },
      { label: "Hidden", field: "hidden" },
      {
        label: "Date",
        value: ({ date }) => new Date(date).toLocaleString(),
        headerRemove: "justify-end",
      },
      {
        field: "Image",
        value: (v) =>
          `<img src="/api/photos/${v.filename}" height="70" alt="${v.filename}">`,
        class: "w-1/2",
        sortable: false,
        headerRemove: "justify-end",
      },
    ]}
  />
</section>
