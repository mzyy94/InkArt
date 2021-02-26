<script lang="ts">
  import { onMount } from "svelte";
  import { Tab, Tabs } from "smelte";
  import PhotoCard from "../molecules/PhotoCard.svelte";
  import PhotoList from "../molecules/PhotoList.svelte";

  let mode = "list";

  let data: { filename: string; date: string; hidden: boolean }[] = [];
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
  <Tabs
    bind:selected={mode}
    color="black"
    notSelectedColor="gray"
    indicator={false}
    items={[
      { id: "grid", text: "grid", icon: "grid_view" },
      { id: "list", text: "list", icon: "list" },
    ]}
  >
    <div slot="content">
      <Tab id="grid" selected={mode}>
        <section class="flex w-full flex-row flex-wrap justify-center">
          {#each data as elm}
            <PhotoCard data={elm} />
          {/each}
        </section>
      </Tab>
      <Tab id="list" selected={mode}>
        <PhotoList bind:data bind:loading />
      </Tab>
    </div>
  </Tabs>
</section>
