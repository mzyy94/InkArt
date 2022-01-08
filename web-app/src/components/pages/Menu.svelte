<script lang="ts">
  import { push } from "svelte-spa-router";
  import { List } from "smelte";
  import Container from "../templates/Container.svelte";

  const menuItems = [
    {
      id: "upload",
      value: "/upload",
      text: "Photo Upload",
      icon: "photo",
      subheading: "Upload and Edit Photo",
    },
    {
      id: "files",
      value: "/files",
      text: "File Management",
      icon: "folder",
      subheading: "Hide or Delete photos and change the display order",
    },
    {
      id: "control",
      value: "/control",
      text: "Display Control",
      icon: "gamepad",
      subheading: "Set display rotation and paddings",
    },
    {
      id: "settings",
      value: "/settings",
      text: "Settings",
      icon: "settings",
      subheading: "Date/Time, Location and settings other than display",
    },
    {
      id: "info",
      value: "/info",
      text: "Information",
      icon: "info",
      subheading: "App Version and Device Information",
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
