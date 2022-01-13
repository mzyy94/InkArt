<script lang="ts">
  import { createEventDispatcher } from "svelte";
  import { DataTable, Image } from "smelte";
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
        component: Image,
        componentProps: (data) => ({
          src: `/api/v1/photos/${data.filename}`,
          alt: data.filename,
          class: "h-24",
        }),
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
