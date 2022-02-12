<nav class="container">
  <ul>
    <!-- svelte-ignore a11y-invalid-attribute -->
    <li><a href="#"><i class="material-icons">home</i></a></li>
  </ul>
  <ul>
    <li><strong><slot name="title">Title</slot></strong></li>
  </ul>
  <ul />
</nav>

<section class="container">
  <slot />
</section>

<style>
  nav {
    margin-bottom: 1em;
  }
</style>
