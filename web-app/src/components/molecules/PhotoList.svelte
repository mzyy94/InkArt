<script lang="ts">
  import { createEventDispatcher } from "svelte";
  import { DataTable } from "smelte";
  import ActionIcon from "../atoms/ActionIcon.svelte";
  import type { Entry } from "../../api";

  const dispatch = createEventDispatcher();

  function hideFile(data: Entry) {
    dispatch("hide", { data });
  }

  function deleteFile(data: Entry) {
    dispatch("delete", { data });
  }

  export let data: Entry[];
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
        componentProps: (data) => ({
          icon: data.hidden ? "visibility_off" : "visibility",
          callback: () => hideFile(data),
        }),
      },
      {
        field: "Image",
        value: (v) =>
          `<img src="/api/v1/photos/${v.filename}" height="70" alt="${v.filename}">`,
        class: "w-1/2",
        sortable: false,
        headerRemove: "justify-end",
      },
      {
        field: "Delete",
        remove: "text-right",
        add: "text-center",
        component: ActionIcon,
        componentProps: (data) => ({
          icon: "delete",
          class: "text-error-500",
          callback: () => deleteFile(data),
        }),
        sortable: false,
      },
    ]}
  />
</section>
