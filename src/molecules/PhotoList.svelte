<script lang="ts">
  import { createEventDispatcher } from "svelte";
  import { DataTable } from "smelte";
  import ActionIcon from "../atoms/ActionIcon.svelte";

  const dispatch = createEventDispatcher();

  function hideFile(data: { filename: string; date: string; hidden: boolean }) {
    dispatch("hide", { data });
  }

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
      {
        field: "Visibility",
        remove: "text-right",
        add: "text-center",
        component: ActionIcon,
        componentProps: (data) => ({icon: data.hidden ? "visibility_off" : "visibility", callback: () => hideFile(data)}),
      },
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
