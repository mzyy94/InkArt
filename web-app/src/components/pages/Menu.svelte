<script lang="ts">
  import { push } from "svelte-spa-router";
  import { List } from "smelte";
  import Container from "../templates/Container.svelte";

  const menuItems = [
    {
      id: "upload",
      value: "/upload",
      text: "Upload photo",
      icon: "photo",
      subheading: "Edit and upload photo",
    },
    {
      id: "files",
      value: "/files",
      text: "File Management",
      icon: "folder",
      subheading: "Organize photos to be shown",
    },
    {
      id: "control",
      value: "/control",
      text: "Display Control",
      icon: "gamepad",
      subheading: "Set display orientation and paddings",
    },
    {
      id: "settings",
      value: "/settings",
      text: "Settings",
      icon: "settings",
      subheading: "Change device Date/Time, refresh interval, etc.",
    },
    {
      id: "info",
      value: "/info",
      text: "Information",
      icon: "info",
      subheading: "App version and device information",
    },
  ];

  const selectListItem = (e: { detail: string }) => {
    const menuItem = menuItems.find(({ id }) => id == e.detail);
    if (menuItem) {
      push(menuItem.value);
    }
  };
</script>

<main>
  <Container>
    <span slot="title">Welcome monophoto6</span>
    <section>
      <List on:change={selectListItem} items={menuItems} class="w-full" />
    </section>
  </Container>
</main>

<style>
</style>
