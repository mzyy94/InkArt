<script lang="ts">
  import { createEventDispatcher } from "svelte";
  import type { Entry } from "../../api";
  import Image from "../atoms/Image.svelte";

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
  <table aria-busy={loading}>
    <thead>
      <tr>
        <th>Name</th>
        <th>Visibility</th>
        <th>Image</th>
        <th>Delete</th>
      </tr>
    </thead>
    <tbody>
      {#each data as entry}
        <tr>
          <td>{entry.filename}</td>
          <td>
            <i class="material-icons" on:click={() => hideFile(entry)}>
              {entry.hidden ? "visibility_off" : "visibility"}
            </i>
          </td>
          <td>
            <Image src={`/api/v1/photos/${entry.filename}`} />
          </td>
          <td>
            <i class="material-icons" on:click={() => deleteFile(entry)}>
              delete
            </i>
          </td>
        </tr>
      {/each}
    </tbody>
  </table>
</section>

<style>
  i {
    cursor: pointer;
  }
  tbody > tr td {
    height: 15vh;
  }
</style>
